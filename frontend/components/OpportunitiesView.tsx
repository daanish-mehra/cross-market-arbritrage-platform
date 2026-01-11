'use client'

interface ArbitrageOpportunity {
  event_id: string
  buy_market: number
  sell_market: number
  buy_price: number
  sell_price: number
  profit_percentage: number
  max_size: number
}

interface OpportunitiesViewProps {
  opportunities: ArbitrageOpportunity[]
}

const marketNames: { [key: number]: string } = {
  0: 'Polymarket',
  1: 'Kalshi',
  2: 'PredictIt'
}

export default function OpportunitiesView({ opportunities }: OpportunitiesViewProps) {
  // Sort opportunities by profit percentage (highest first)
  const sortedOpportunities = [...opportunities].sort((a, b) => b.profit_percentage - a.profit_percentage)

  return (
    <>
      {/* Header */}
      <div className="page-header">
        <div>
          <h1 className="page-title">Arbitrage Opportunities</h1>
          <p className="page-subtitle">CROSS-MARKET PRICE DISCREPANCIES // REAL-TIME DETECTION</p>
        </div>
        {sortedOpportunities.length > 0 && (
          <div style={{ display: 'flex', alignItems: 'center', gap: '16px' }}>
            <div style={{ textAlign: 'right' }}>
              <div style={{ fontSize: '12px', color: 'var(--text-muted)', textTransform: 'uppercase', letterSpacing: '0.05em' }}>
                Active Opportunities
              </div>
              <div style={{ fontSize: '24px', fontWeight: 700, color: 'var(--primary)', fontFamily: 'monospace' }}>
                {sortedOpportunities.length}
              </div>
            </div>
          </div>
        )}
      </div>

      {/* Opportunities List */}
      <div style={{ padding: '0 32px 32px' }}>
        {sortedOpportunities.length === 0 ? (
          <div className="empty-state-card">
            <div className="empty-state-icon">🔍</div>
            <div className="empty-state-title">No Arbitrage Opportunities Detected</div>
            <div className="empty-state-subtitle">
              Waiting for cross-market price discrepancies...
              <br />
              Opportunities will appear here when the same event has different prices across markets.
            </div>
          </div>
        ) : (
          <div className="opportunities-grid">
            {sortedOpportunities.map((opp, index) => {
              const profitPercent = opp.profit_percentage * 100
              const isHighProfit = profitPercent > 2.0
              
              return (
                <div key={index} className="opportunity-card">
                  <div className="opportunity-header">
                    <div className="opportunity-title-group">
                      <h3 className="opportunity-title">{opp.event_id}</h3>
                      <p className="opportunity-subtitle">
                        Buy on {marketNames[opp.buy_market] || `Market ${opp.buy_market}`} • Sell on {marketNames[opp.sell_market] || `Market ${opp.sell_market}`}
                      </p>
                    </div>
                    <div className={`profit-badge ${isHighProfit ? 'high' : 'medium'}`}>
                      +{profitPercent.toFixed(2)}%
                    </div>
                  </div>

                  <div className="opportunity-details-grid">
                    <div className="opportunity-detail-item">
                      <span className="opportunity-detail-label">Buy Price</span>
                      <span className="opportunity-detail-value">{(opp.buy_price * 100).toFixed(2)}¢</span>
                    </div>
                    <div className="opportunity-detail-item">
                      <span className="opportunity-detail-label">Sell Price</span>
                      <span className="opportunity-detail-value sell">{(opp.sell_price * 100).toFixed(2)}¢</span>
                    </div>
                    <div className="opportunity-detail-item">
                      <span className="opportunity-detail-label">Max Size</span>
                      <span className="opportunity-detail-value">${opp.max_size.toFixed(2)}</span>
                    </div>
                    <div className="opportunity-detail-item">
                      <span className="opportunity-detail-label">Net Profit</span>
                      <span className="opportunity-detail-value profit">
                        ${((opp.sell_price - opp.buy_price) * opp.max_size).toFixed(2)}
                      </span>
                    </div>
                  </div>

                  <div className="opportunity-actions">
                    <button className="opportunity-button primary">
                      Execute Trade
                    </button>
                  </div>
                </div>
              )
            })}
          </div>
        )}
      </div>
    </>
  )
}

